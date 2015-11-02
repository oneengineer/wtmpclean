/**
 * Created by xiaochen.tian on 10/19/2015.
 */


import org.joda.time.format.{DateTimeFormat, DateTimeFormatter}
import org.joda.time.{DateTimeZone, DateTime}

import scala.sys._

import scala.sys.process._
import scala.util.Random

val jst = org.joda.time.DateTimeZone.forOffsetHours(9)

def modify_time(filename:String,pp:String = "%y"):DateTime = {
  try {
    val w = s"stat -c $pp $filename" !!

    val w2 = w.split('.').apply(0)

    val p = DateTimeFormat.forPattern("yyyy-MM-dd HH:mm:ss")
    p.parseDateTime(w2).toDateTime(jst)
  }
  catch {
    case _:Throwable => new DateTime(2016,1,1,1,1)
  }
}



import java.io.File

def recursiveListFiles(f: File): Vector[File] = {
  val these = f.listFiles toVector

  these ++ these.filter(_.isDirectory).flatMap(recursiveListFiles)
}

def files(path:String) = recursiveListFiles(new File(path))

val dtf = DateTimeFormat.forPattern("yyyy-MM-dd HH:mm:ss");

def crt_time(path:String):DateTime = {

  val exe = "/home/xchen/shared/sparkjar/crt.sh"
  //val exe = "/home/semuser/scripts/crt.sh"
  try {

    val t = Seq(exe, path) !!

    val t2 = t.trim.split(" ") filter (_.length>0)

    t2(2) = if (t2(2).length <2) "0" + t2(2) else t2(2)

    val t3 = t2 mkString " "

    val f2 = DateTimeFormat.forPattern("EEE MMM dd HH:mm:ss yyyy")

    val dt = f2.parseDateTime(t3).toDateTime(jst)

    dt
  }   catch {
    case _:Throwable => new DateTime(2016,1,1,1,1)
  }
}


def create_time(month:Int,day:Int,hour:Int,min:Int):String = {
  val sec = Random.nextInt(60)
  val msec = Random.nextInt(999999999)

  val x = new DateTime(2015,month,day,hour,min,sec,org.joda.time.DateTimeZone.forOffsetHours(9))

  val s1 = x.toString(dtf)
  val s2 = f"${msec}%09d"
  s"$s1.$s2 +0900"
}

def change_modify_time_a(month:Int,day:Int,hour:Int,min:Int,path:String ):Unit = {
  val t = create_time(month,day,hour,min)
  println(s"DEBUG  $t   $path")

  val now = (Seq("/bin/date","+%Y-%m-%d %H:%M:%S") !!).trim

  println("now ",now)

  Seq("sudo","date","-s",t.trim) !

  Seq("touch","-d",t,path) !
  //println(Seq("sudo","date","-s",now).mkString(" ")) //debug

  Seq("sudo","date","-s",now) !
}


def change_modify_time( t:String,path:String ):Unit = {
  println(s"DEBUG  $t   $path")

  val now = (Seq("/bin/date","+%Y-%m-%d %H:%M:%S") !!).trim

  println("now ",now)

  Seq("sudo","date","-s",t.trim) !

  Seq("touch","-m","-d",t,path) !
  //println(Seq("sudo","date","-s",now).mkString(" ")) //debug

  Seq("sudo","date","-s",now) !
}

def change_modify_time2(month:Int,day:Int,hour:Int,min:Int,path:String ):Unit = {
  val t = create_time(month,day,hour,min)
  change_modify_time(t,path)
}


def change_access_time( t:String,path:String ):Unit = {
  val cmd = Seq("touch","-a","-d",t,path)
  cmd !
}

def change_access_time2(month:Int,day:Int,hour:Int,min:Int,path:String ):Unit = {
  val t = create_time(month,day,hour,min)
  change_access_time(t,path)
}

case class AA(path:String,month:Int,day:Int,hour:Int,min:Int)

val aas = Vector(
  AA("/home/tianxiaochen01/data/rat.final",9,1,13,21),
  AA("/home/tianxiaochen01/data/out.vector2",9,1,18,13),
  AA("/home/tianxiaochen01/result2",5,29,18,55),
  AA("/home/tianxiaochen01/rat_memo",6,8,10,8),
  AA("/home/semuser/spark/conf/spark-env.sh",7,16,18,19),
  AA("/home/semuser/code/ads_clustering/data/out.vector2",8,11,14,30)
)

val allu = Vector(
  AA("/home/tianxiaochen01/pypi",6,4,10,8),
  AA("/home/tianxiaochen01/mpi/myprogram/prototype.cpp",7,1,11,9),
  AA("/home/tianxiaochen01/mpi/myprogram/verysimple.cpp",7,3,10,27),
  AA("/home/tianxiaochen01/mpi/myprogram/lesson03_power_method.cpp",6,26,18,29),

  AA("/home/semuser/spark-1.4.1/conf/spark-env.sh",7,16,18,38),
  AA("/home/semuser/spark-1.4.1/sbin/spark-config.sh",7,17,17,29),

  AA("/home/tianxiaochen01/mpi/myprogram/main.cpp",9,8,18,59),

  AA("/home/semuser/spark-1.4.1/examples/src/main/scala/org/apache/spark/examples/SparkLR.scala",7,21,18,20),
  AA("/home/semuser/spark-1.4.1/examples/src/main/scala/org/apache/spark/examples/SparkPageRank.scala",7,21,16,20),
  AA("/home/semuser/spark-1.4.1/examples/src/main/scala/org/apache/spark/examples/LocalALS.scala",7,21,17,55),
  AA("/home/semuser/spark-1.4.1/examples/src/main/scala/org/apache/spark/examples/SimpleSkewedGroupByTest.scala",7,21,18,1),

  AA("/home/semuser/spark-1.4.1/examples/src/main/scala/org/apache/spark/examples/HdfsTest.scala",7,24,18,2),
  AA("/home/semuser/spark-1.4.1/examples/src/main/scala/org/apache/spark/examples/SparkKMeans.scala",7,24,15,21),
  AA("/home/semuser/spark-1.4.1/examples/src/main/scala/org/apache/spark/examples/LocalLR.scala",7,24,17,39)

)

//access
val osa = Vector(
  AA("/home/tianxiaochen01/data/rat.final",9,11,19,1),
  AA("/home/tianxiaochen01/data/out.vector2",9,4,10,22),
  AA("/home/tianxiaochen01/result2",6,20,15,39),
  AA("/home/tianxiaochen01/rat_memo",6,17,9,54),
  AA("/home/semuser/spark/conf/spark-env.sh",7,16,18,25),
  AA("/home/semuser/code/recommendation/data/trilinosExample_cluster",8,21,18,24),
  AA("/home/semuser/code/ads_clustering/data/out.vector2",8,20,19,30)

)

def batch_update_modify() = {
  aas foreach { x=>
    change_modify_time2(x.month,x.day,x.hour,x.min,x.path)
  }
}

def all_u_batch() = {
  allu foreach { x=>
    change_modify_time_a(x.month,x.day,x.hour,x.min,x.path)
  }
}


def batch_a() = {
  osa foreach { x=>
    change_access_time2(x.month,x.day,x.hour,x.min,x.path)
  }
}


def go = {
  batch_a

  batch_update_modify

  all_u_batch
}

go

def printToFile(f: java.io.File)(op: java.io.PrintWriter => Unit) {
  val p = new java.io.PrintWriter( new java.io.OutputStreamWriter(new java.io.FileOutputStream(f),"UTF-8") )
  try { op(p) } finally { p.close() }
}


def writeToFile(str:String,fileName:String) = {
  printToFile(new File(fileName)) { p =>
    p.println(str)
  }
}

def get_all() = {
  import com.github.nscala_time.time.Imports._

  val paths = files("/home/tianxiaochen01") map (_.getAbsolutePath)
  //val paths = files("/home/semuser") map (_.getAbsolutePath)

  val data = paths map { x => (
    x,
    crt_time(x),
    modify_time(x),   // modify
    modify_time(x,"%x"), //access
    modify_time(x,"%z") //change
    )
  } sortBy (_._2)

  val strs = data map {
    x =>
      //println(x)
      x
  } mkString "\n"

  //writeToFile(strs,"files_t.all")

  data

}

val data = get_all()

import com.github.nscala_time.time.Imports._

val start = new DateTime(2015,7,10,0,0,0)
val end = new DateTime(2015,7,17,0,0,0)
//data filter ( x => start <= x._2 && x._2 <= end ) foreach println

data filter ( x => start <= x._2 && x._2 <= end && (x._1 contains "example" ) ) foreach println



//data filter ( x => start <= x._2 && x._2 <= end ) length
//
//
//data filter ( x=> Seq(x._2.getHourOfDay,x._3.getHourOfDay,x._4.getHourOfDay,x._5.getHourOfDay).max > 18  ) foreach println
//
//data filter ( x=> Seq(x._2.getHourOfDay,x._3.getHourOfDay,x._4.getHourOfDay,x._5.getHourOfDay).max > 18  ) length
//
//
//data filter ( x=> x._2.getHourOfDay > 18  ) foreach println
//
//

