
import org.joda.time.format.{DateTimeFormat, DateTimeFormatter}
import org.joda.time.{DateTimeZone, DateTime}

import scala.sys._

import scala.sys.process._
import scala.util.Random

val cmd = "sbin/wtmpclean"
val file = "wtmp2"

def add(ut_line:String,start:DateTime,end:DateTime,host:String,ip:String,user:String): Unit ={
  val ips = ip.split('.')
  //val ip_int = ips(0).toInt*256*256*256 + ip(1).toInt*256*256+ip(2).toInt*256 + ip(3).toInt
  val ip_int = ip
  val s = Seq( cmd,"-A","-f",file,
    "--line",ut_line,
    "--start",(start.getMillis/1000).toString,
    "--end",(end.getMillis/1000).toString,
    "-i",ip_int.toString,
    "-H",host,user)

  println(s.mkString(" "))

  val result = s !!

  println(result)
}

def edit(start:Int,end:DateTime,host:String,ip:String,user:String,pid:Int,isEnd:Boolean): Unit ={
  val ips = ip.split('.')

  var s = Seq( cmd,"-E","-f",file,
    "--start",start.toString,
    "--end",(end.getMillis/1000).toString,
    "-p",pid.toString)

  if (ip.length > 0){
    //val ip_int = ips(0).toInt*256*256*256 + ip(1).toInt*256*256+ip(2).toInt*256 + ip(3).toInt
    val ip_int = ip
    s = s ++ Seq("-i",ip_int.toString)
  }

  if (host.length > 0){
    s = s ++ Seq("-H",host)
  }
  if (user.length > 0){
    s = s ++ Seq("-H",host)
  }
  if (isEnd){
    s = s ++ Seq("-d")
  }

  println(s.mkString(" "))

  val result = s !!

  println(result)
}

val jst = org.joda.time.DateTimeZone.forOffsetHours(9)


def gett(month:Int,day:Int,hour:Int,min:Int,sec:Int = 0) = {
  if (sec > 0)
  new DateTime( 2015,month,day,hour,min,sec,jst)
  else new DateTime( 2015,month,day,hour,min,Random.nextInt(55)+1,jst)
}

val editseq = Seq(
  //start time, end time, host,ip,user,pid,isEnd
)

val addseq = Seq(
)

def work = {
  "cp abcd wtmp2" !!

  for (
    i<- editseq
  ) {
    (edit _).tupled.apply(i)
    "cp generated_wtmp wtmp2" !

  }

  for (
    i<- addseq
  ) {
    (add _).tupled.apply(i)
    "cp generated_wtmp wtmp2" !

  }

}



work





