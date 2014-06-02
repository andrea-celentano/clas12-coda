      subroutine ecbookrzhraw()

      call hmdir('//PAWC/EC','S')

      call hbook1(770,'',20400,0.,20401.,0.)
      call hbook1(771,'',20400,0.,20401.,0.)
      call hbook1(772,'',20400,0.,20401.,0.)

      call hcdir('//PAWC',' ')

      return
      end
