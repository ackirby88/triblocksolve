!
! File:   read_mesh_data.F90
! Author: akirby
!
! Created on September 28, 2021, 4:26 PM
!

subroutine read_mesh_data(nvar,nelem,nintface,eftot,nline,nlineelem, &
                          epoint,ef,fc,lines,linesize,linepoint,lineface) &
                          bind(C,name="read_mesh_data_")
    use iso_c_binding
    implicit none

    integer, intent(in) :: nvar,nelem,nintface,eftot,nline,nlineelem
    integer, intent(out):: epoint(nelem+1)
    integer, intent(out):: ef(eftot)
    integer, intent(out):: fc(2,nintface)
    integer, intent(out):: lines(nlineelem)
    integer, intent(out):: linesize(nline)
    integer, intent(out):: linepoint(nline+1)
    integer, intent(out):: lineface(nlineelem)

    integer :: ierror
    integer :: nvar_,nelem_,nintface_,eftot_,nline_,nlineelem_
    integer :: i,j

    open(unit=90,file='gpuline.mesh.data.bin',status='old',iostat=ierror,access='stream',form='unformatted')
    read(90) nvar_,nelem_,nintface_,eftot_,nline_,nlineelem_
    read(90) epoint
    read(90) ef
    read(90) fc
    read(90) linesize
    read(90) linepoint
    read(90) lines
    read(90) lineface
    close(90)

    ! ========================= !
    ! remove fortran base index !
    ! ========================= !
    do i = 1,nelem+1
        epoint(i) = epoint(i)-1
    end do

    do i = 1,eftot
        ef(i) = ef(i)-1
    end do

    do i = 1,nintface
        fc(1,i) = fc(1,i)-1
        fc(2,i) = fc(2,i)-1
    end do

    do i = 1,nline+1
        linepoint(i) = linepoint(i)-1
    end do

    do i = 1,nlineelem
        lines(i) = lines(i)-1
    end do

    do i = 1,nlineelem
        lineface(i) = lineface(i)-1
    end do

end subroutine