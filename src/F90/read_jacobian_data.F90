!
! File:   read_jacobian_data.F90
! Author: akirby
!
! Created on September 29, 2021, 9:25 AMs
!

subroutine read_jacobian_data(nvar,nelem,nintface, &
                             jacD,jacO1,jacO2, &
                             rhs,U0) &
                             bind(C,name="read_jacobian_data_")
    use iso_c_binding
    implicit none

    integer,intent(in) :: nvar,nelem,nintface
    real*8, intent(out):: jacD(nvar*nvar*nelem)
    real*8, intent(out):: jacO1(nvar*nvar*nintface)
    real*8, intent(out):: jacO2(nvar*nvar*nintface)
    real*8, intent(out):: rhs(nvar*nelem)
    real*8, intent(out):: U0(nvar*nelem)

    integer :: ierror
    integer :: nvar_,nelem_,nintface_

    open(unit=90,file="gpuline.jacobian.data.bin",status='old',iostat=ierror,access='stream',form='unformatted')
    read(90) nvar_,nelem_,nintface_
    read(90) jacD
    read(90) jacO1
    read(90) jacO2
    read(90) rhs
    read(90) U0
    close(90)
end subroutine