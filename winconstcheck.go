// 4 june 2014
// based on code from 24 may 2014
package main

import (
	"fmt"
	"os"
	"strings"
	"go/token"
	"go/ast"
	"go/parser"
	"code.google.com/p/go.tools/go/types"
	_ "code.google.com/p/go.tools/go/gcimporter"
)

var arch string

func getPackage(path string) (typespkg *types.Package, pkginfo types.Info) {
	var pkg *ast.Package

	fileset := token.NewFileSet()		// parser.ParseDir() actually writes to this; not sure why it doesn't return one instead
	filter := func(i os.FileInfo) bool {
		if strings.Contains(i.Name(), "_windows") &&
			strings.Contains(i.Name(), "_" + arch) &&
			strings.HasSuffix(i.Name(), ".go") {
			return true
		}
		if i.Name() == "race.go" ||		// skip these
			i.Name() == "flock.go" {
			return false
		}
		return strings.HasSuffix(i.Name(), "_windows.go") ||
			(!strings.Contains(i.Name(), "_"))
	}
	pkgs, err := parser.ParseDir(fileset, path, filter, parser.AllErrors)
	if err != nil {
		panic(err)
	}
	for k, _ := range pkgs {		// get the sole key
		if pkgs[k].Name == "syscall" {
			pkg = pkgs[k]
			break
		}
	}
	if pkg == nil {
		panic("package syscall not found")
	}
	// we can't pass pkg.Files directly to types.Check() because the former is a map and the latter is a slice
	ff := make([]*ast.File, 0, len(pkg.Files))
	for _, v := range pkg.Files {
		ff = append(ff, v)
	}
	// if we don't make() each map, package types won't fill the structure
	pkginfo.Defs = make(map[*ast.Ident]types.Object)
	pkginfo.Scopes = make(map[ast.Node]*types.Scope)
	typespkg, err = new(types.Config).Check(path, fileset, ff, &pkginfo)
	if err != nil {
		panic(err)
	}
	return typespkg, pkginfo
}

func main() {
	pkgpath := "/home/pietro/go/src/pkg/syscall"
	arch = os.Args[1]

	pkg, _ := getPackage(pkgpath)
	scope := pkg.Scope()
	for _, name := range scope.Names() {
		obj := scope.Lookup(name)
		if obj == nil {
			panic(fmt.Errorf("nil object %q from scope %v", name, scope))
		}
		if !obj.Exported() {		// exported names only
			continue
		}
		if _, ok := obj.(*types.Const); ok {
			fmt.Printf("egrep -rh '#define[ 	]+%s' ~/winshare/Include/ 2>/dev/null\n", obj.Name())
		}
		// otherwise skip
	}
}
