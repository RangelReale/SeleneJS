objs = {}
function make_ten() {
   for (i = 1; i<=10; i++) {
      objs[i] = new GCTest()
   }
}

function destroy_ten() {
   for (i = 1; i<=10; i++) {
      objs[i] = undefined
   }
}
