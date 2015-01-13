// 12 november 2014
package main
import "github.com/andlabs/ui"
import "reflect"
var w ui.Window
type A struct {
	A	bool
	B	bool
}
func main() {
	go ui.Do(func() {
		t := ui.NewTable(reflect.TypeOf(A{}))
		t.Lock()
		d := t.Data().(*[]A)
		*d = make([]A, 20)
		t.Unlock()
		w = ui.NewWindow("Hi", 300, 200, t)
		w.Show()
	})
	ui.Go()
}
