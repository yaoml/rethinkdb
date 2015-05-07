// Autogenerated by nvert_protofile.py on 2015-05-06.
// Do not edit this file directly.
// The template for this file is located at:
// ../../../../../../../../templates/AstSubclass.java
package com.rethinkdb.ast.gen;

import com.rethinkdb.ast.helper.Arguments;
import com.rethinkdb.ast.helper.OptArgs;
import com.rethinkdb.ast.RqlAst;
import com.rethinkdb.proto.TermType;
import java.util.*;



public class SetIntersection extends RqlQuery {


    public SetIntersection(java.lang.Object arg) {
        this(new Arguments(arg), null);
    }
    public SetIntersection(Arguments args, OptArgs optargs) {
        this(null, args, optargs);
    }
    public SetIntersection(RqlAst prev, Arguments args, OptArgs optargs) {
        this(prev, TermType.SET_INTERSECTION, args, optargs);
    }
    protected SetIntersection(RqlAst previous, TermType termType, Arguments args, OptArgs optargs){
        super(previous, termType, args, optargs);
    }

    public static SetIntersection fromArgs(Object... args){
        return new SetIntersection(new Arguments(args), null);
    }

}