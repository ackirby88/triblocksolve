subroutine linesmoothLU(btot,etot,fctot,eftot,epoint,ef,fc,lintot,linelemtot,linsize,linpoint,lines,linface,iters,Dia,Of1,Of2,B,Ui,U,R)
implicit none
integer, intent(in) :: btot,etot,fctot, eftot
integer, intent(in) :: fc(2,fctot), epoint(etot+1),ef(eftot)

integer, intent(in) :: lintot, linelemtot
integer, intent(in) :: linsize(lintot), linpoint(lintot+1)
integer, intent(in) :: lines(linelemtot), linface(linelemtot)

integer, intent(in) :: iters
real(8), intent(in) :: Dia(btot,btot,etot)
real(8), intent(in) :: Of1(btot,btot,fctot),Of2(btot,btot,fctot)
real(8), intent(in) :: B(btot,etot),Ui(btot,etot)
real(8), intent(out) :: U(btot,etot),R(btot,etot)

real(8) dU(btot,etot)
integer k
integer e,f,e1,e2,i,j,p,elast,k2
integer m, l
real(8) ftemp(btot),LU(btot,btot)

real(8) S(btot),dUtemp(btot)

real(8) rms

integer :: ierror
character(80) :: filename
character(80) :: file_num


!$omp parallel do
DO e=1,etot
   U(:,e)=Ui(:,e)
   R(:,e)=B(:,e)
end do
!$omp end parallel do

! ======================= !
! WRITE DATA FOR GPU PORT !
! ======================= !
! A. Line Header Information
!   1. lintot               number of lines
!   2. linelemtot           number of line elements total
!   3. linpoint(lintot+1)   line start index into line-element list (m=linpoint(l) where l is the line number)
!   4. linsize(lintot)      number of elements in each line
!   5. lines(linelemtot)    line-element list
!   6. linface(linelemtot)  line face index connecting elements (f=linface(m); e1=fc(1,f); e2=fc(2,f))
!
! B. Mesh and Solver Data
!   1. btot             number of variables in block (9)
!   2. etot             number of elements
!   3. fctot            number of interior faces
!   4. eftot            number of ???? (FIXME)
!   5. fc(2,fctot)      ????
!   6. epoint(etot+1)
!   7. ef(eftot)
!   8. Dia

open(unit=90,file='gpuline.mesh.data',status='replace',iostat=ierror)
write(90,*) "       nvar       nelem   nintfaces       eftot      nlines  nlineelem"
write(90,*) btot,etot,fctot,eftot,lintot,linelemtot

write(90,*) "epoint(nelem+1): "
write(90,*) epoint

write(90,*) "ef(eftot): "
write(90,*) ef

write(90,*) "fc(2,nintfaces): face-element connectivity (fc(1): left element, fc(2): right element)"
write(90,*) fc

write(90,*) "linesize(nlines): number of elements in each line"
write(90,*) linsize

write(90,*) "linepoint(nlines+1): line start index into line-element list (m=linepoint(l) where l is the line number)"
write(90,*) linpoint

write(90,*) "lines(nlineelem): line-element list (all line elements stacked contiguously)"
write(90,*) lines

write(90,*) "lineface(nlineelem): line face index connecting elements (f=linface(m); e1=fc(1,f); e2=fc(2,f))"
write(90,*) linface
close(90)

open(unit=90,file='gpuline.jacobian.data',status='replace',iostat=ierror)
write(90,*) "       nvar       nelem   nintfaces"
write(90,*) btot,etot,fctot
write(90,*) "jacDiag(nvar,nvar,nelem): Jacobian Diagonal Blocks (LU factored)"
write(90,*) Dia

write(90,*) "jacO1(nvar,nvar,nintfaces): Jacobian Off Diagonal Blocks 1"
write(90,*) Of1

write(90,*) "jacO2(nvar,nvar,nintfaces): Jacobian Off Diagonal Blocks 2"
write(90,*) Of2

write(90,*) "B(nvar,nelem): right-hand-side vector of linear system"
write(90,*) B

write(90,*) "Ui(nvar,nelem): initial solution guess for linear system"
write(90,*) Ui
close(90)

DO p=1,iters
  !---------------------------------------------------

   !---------------------------------------------------
   !   Solve for dU using Line relaxation and Jacobi on non-lines
   !---------------------------------------------------
   !   NOTE: On first pass of Jacobi method (non-lines), the linear residual
   !         only includes the spatial residual (does not include -J*U).
   !         After the 1st pass, Linear Residual Calculation includes these terms.

   !$omp parallel do private(S,k,m,e,elast,f,e1,e2,dutemp)
   DO l=1,lintot
      if (linsize(l)==1) then
         m=linpoint(l)
         e=lines(m)
         S(:)=-R(:,e)
         call solveLU(Dia(:,:,e),S,btot,dU(:,e)) !bug on Dia?
      else
         !  Perform Forward and Backward Substitution of Thomas Algorithm
         m=linpoint(l)
         e=lines(m)
         S(:)=-R(:,e)
         call solveLU(Dia(:,:,e),S(:),btot,dU(:,e))

         DO k=2,linsize(l)
            m=linpoint(l)+k-1
            e=lines(m)
            f=linface(m)
            e1=fc(1,f)
            e2=fc(2,f)

            m=linpoint(l)+k-2
            elast=lines(m)
            if (e==e1) then
               S(:)=-R(:,e)-matmul(Of2(:,:,f),dU(:,elast))
            else
               S(:)=-R(:,e)-matmul(Of1(:,:,f),dU(:,elast))
            end if
            call solveLU(Dia(:,:,e),S(:),btot,dU(:,e))
         end do
         DO k=linsize(l)-1,1,-1
            m=linpoint(l)+k-1
            e=lines(m)
            f=linface(m+1)
            e1=fc(1,f)
            e2=fc(2,f)

            m=linpoint(l)+k
            elast=lines(m)
            if (e==e1) then
               dUtemp(:)=matmul(Of2(:,:,f),dU(:,elast))
            else
               dUtemp(:)=matmul(Of1(:,:,f),dU(:,elast))
            end if
            call solveLU(Dia(:,:,e),dUtemp(:),btot,S(:))
            dU(:,e)=dU(:,e)-S(:)
         end do
      end if
   END DO
   !$omp end parallel do

   write(file_num,'(I2.2)') p
   filename = 'gpu.dU.info.' // adjustr(trim(file_num))

    open(unit=91,file=filename,status='replace',iostat=ierror)
    write(91,*) "       nvar       nelem"
    write(91,*)  btot,etot
    write(91,*) "dU(nvar,nelem): dU update for linear system"
    write(91,*) dU
    close(91)

   !$omp parallel do
   DO e=1,etot
      U(:,e)=U(:,e)+dU(:,e)
   END DO
   !$omp end parallel do

   !------------------------------------------------
   !  Calculate Linear Residual
   !------------------------------------------------
   !$omp parallel do
   DO e=1,etot
      R(:,e)=B(:,e)
   END DO
   !$omp end parallel do

   !$omp parallel do private(m,e,k,f,e1,e2,i,j,ftemp,k2,S,dUtemp)
   DO l=1,lintot
      if (linsize(l)==1) then
         m=linpoint(l)
         e=lines(m)
         DO k=epoint(e),epoint(e+1)-1
            f=ef(k)
            if (f>0) then
               e1=fc(1,f)
               e2=fc(2,f)
               if (e1==e) then
                  DO i=1,btot
                     DO j=1,btot
                        R(i,e)=R(i,e)+Of2(i,j,f)*U(j,e2)
                     END DO
                  END DO
               else
                  DO i=1,btot
                     DO j=1,btot
                        R(i,e)=R(i,e)+Of1(i,j,f)*U(j,e1)
                     END DO
                  END DO
               end if
            end if
         end do
         call LUmatmul(btot,Dia(:,:,e),U(:,e),ftemp(:))
         R(:,e)=R(:,e)+ftemp(:)
      else
         !  Perform Forward and Backward Substitution of Thomas Algorithm
         m=linpoint(l)
         e=lines(m)
         DO k2=epoint(e),epoint(e+1)-1
            f=ef(k2)
            if (f>0) then
               e1=fc(1,f)
               e2=fc(2,f)
               if (e1==e) then
                  DO i=1,btot
                     DO j=1,btot
                        R(i,e)=R(i,e)+Of2(i,j,f)*U(j,e2)
                     END DO
                  END DO
               else
                  DO i=1,btot
                     DO j=1,btot
                        R(i,e)=R(i,e)+Of1(i,j,f)*U(j,e1)
                     END DO
                  END DO
               end if
            end if
         end do
         call LUmatmul(btot,Dia(:,:,e),U(:,e),ftemp(:))
         R(:,e)=R(:,e)+ftemp(:)

         DO k=2,linsize(l)
            m=linpoint(l)+k-1
            e=lines(m)

            f=linface(m)
            e1=fc(1,f)
            e2=fc(2,f)

            m=linpoint(l)+k-2
            elast=lines(m)
            if (e==e1) then
               dUtemp(:)=matmul(Of1(:,:,f),U(:,e))
               call solveLU(Dia(:,:,elast),dUtemp(:),btot,S(:))
               dUtemp(:)=matmul(Of2(:,:,f),S(:))
               call LUmatmul(btot,Dia(:,:,e),U(:,e),S(:))
               R(:,e)=R(:,e)+dUtemp(:)+S(:)
            else
               dUtemp(:)=matmul(Of2(:,:,f),U(:,e))
               call solveLU(Dia(:,:,elast),dUtemp(:),btot,S(:))
               dUtemp(:)=matmul(Of1(:,:,f),S(:))
               call LUmatmul(btot,Dia(:,:,e),U(:,e),S(:))
               R(:,e)=R(:,e)+dUtemp(:)+S(:)
            end if

            DO k2=epoint(e),epoint(e+1)-1
               f=ef(k2)
               if (f>0) then
                  e1=fc(1,f)
                  e2=fc(2,f)
                  if (e1==e) then
                     DO i=1,btot
                        DO j=1,btot
                           R(i,e)=R(i,e)+Of2(i,j,f)*U(j,e2)
                        END DO
                     END DO
                  else
                     DO i=1,btot
                        DO j=1,btot
                           R(i,e)=R(i,e)+Of1(i,j,f)*U(j,e1)
                        END DO
                     END DO
                  end if
               end if
            end do

         end do
      end if
   END DO
   !$omp end parallel do

END DO


stop

return
end subroutine
