
import org.joda.time.format.{DateTimeFormat, DateTimeFormatter}
import org.joda.time.{DateTimeZone, DateTime}

import scala.sys._

import scala.sys.process._
import scala.util.Random

val cmd = "/home/tianxiaochen01/h/sbin/wtmpclean"
val file = "/home/tianxiaochen01/h/wtmp2"

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
  ( 1426238869,gett(3,13,18,32),"","","",3386,true ),
  ( 1427164491,gett(3,24,9,10),"","","",3707,false ),
  ( 1427420383,gett(3,27,9,45),"","","",23514,false),

  ( 1430356557,gett(4,30,10,9),"","","",14083,false),
  ( 1430356641,gett(4,30,10,11),"","","",14083,true),

  ( 1431652529,gett(5,15,10,20),"loginjp101c.db.rakuten.co.jp","16850092","",10466,false),

  ( 1432863877,gett(5,29,9,44,37),"loginjp101c.db.rakuten.co.jp","16850092","",3454,false),
  ( 1432870768,gett(5,29,10,39,28),"","","",3454,true),

  ( 1433729242,gett(6,8,10,3),"loginjp101c.db.rakuten.co.jp","16850092","",7420,false),
  ( 1433729333,gett(6,8,10,16),"","","",7420,true),

  ( 1433900771,gett(6,10,9,58),"","","",7723,false),
  ( 1433900926,gett(6,10,15,28),"","","",7723,true),

  ( 1434008131,gett(6,11,8,45),"","","",5427,false),
  ( 1434009638,gett(6,11,12,49),"","","",5427,true),

  ( 1435890049,gett(7,3,10,54),"","","",29672,false),
  ( 1435920598,gett(7,3,15,27),"","","",29672,true)

)

val addseq = Seq(
  //start time, end time, host,ip,user
  ("pts/9",gett(3,20,18,1),gett(3,20,18,46),"loginjp101c.db.rakuten.co.jp","16850092","tianxiaochen01" ),
  ("pts/0",gett(3,26,9,31),gett(3,26,15,24),"localhost","16777343","tianxiaochen01" ),
  ("pts/9",gett(4,8,20,1),gett(4,8,20,14),"loginjp101c.db.rakuten.co.jp","16850092","tianxiaochen01" ),
  ("pts/0",gett(4,10,20,1),gett(4,10,20,14),"loginjp101c.db.rakuten.co.jp","16850092","tianxiaochen01" ),
  ("pts/0",gett(4,13,18,45),gett(4,13,20,2),"localhost","16777343","tianxiaochen01" ),
  ("pts/0",gett(4,22,15,40),gett(4,22,18,3),"localhost","16777343","tianxiaochen01" ),

  ("pts/0",gett(5,12,20,6),gett(5,12,20,33),"localhost","16777343","tianxiaochen01" ),
  ("pts/0",gett(5,22,10,49),gett(5,22,14,53),"loginjp101c.db.rakuten.co.jp","16850092","tianxiaochen01" ),

  ("pts/0",gett(5,22,10,49),gett(5,22,14,53),"localhost","16777343","tianxiaochen01" ),

  ("pts/0",gett(6,5,10,7),gett(6,5,10,22),"localhost","16777343","tianxiaochen01" ),
  ("pts/0",gett(6,5,13,49),gett(6,5,15,55),"loginjp201zd.zd.rakuten.co.jp","1831998124","tianxiaochen01" ),

  ("pts/0",gett(6,5,13,49),gett(6,5,15,55),"loginjp201zd.zd.rakuten.co.jp","1831998124","tianxiaochen01" ),

  ("pts/27",gett(6,24,10,10),gett(6,24,10,14),"loginjp101c.db.rakuten.co.jp","16850092","tianxiaochen01" ),

  ("pts/27",gett(6,25,11,15),gett(6,25,11,37),"loginjp201zd.zd.rakuten.co.jp","1831998124","tianxiaochen01" ),
  ("pts/27",gett(6,25,13,10),gett(6,25,15,50),"localhost","16777343","tianxiaochen01" ),

  ("pts/27",gett(6,26,12,10),gett(6,25,12,33),"localhost","16777343","tianxiaochen01" ),

  ("pts/18",gett(7,3,13,11),gett(7,3,12,33),"localhost","16777343","tianxiaochen01" ),

  ("pts/18",gett(7,3,20,13),gett(7,3,20,45),"localhost","16777343","tianxiaochen01" ),

  ("pts/18",gett(7,16,16,41),gett(7,16,18,40),"localhost","16777343","tianxiaochen01" ),

  ("pts/18",gett(7,21,9,54),gett(7,21,10,2),"localhost","16777343","tianxiaochen01" ),

  ("pts/18",gett(7,22,10,14),gett(7,22,11,12),"loginjp101z.prod.jp.local","1643659876","tianxiaochen01" ),

  ("pts/18",gett(7,23,14,56),gett(7,23,18,15),"loginjp101z.prod.jp.local","1643659876","tianxiaochen01" ),

  ("pts/30",gett(7,31,9,54),gett(7,31,10,2),"localhost","16777343","tianxiaochen01" ),

  ("pts/37",gett(8,3,11,10),gett(8,3,11,12),"localhost","16777343","tianxiaochen01" ),

  ("pts/22",gett(8,5,11,39),gett(8,5,11,40),"loginjp101z.prod.jp.local","1643659876","tianxiaochen01" ),

  ("pts/33",gett(8,21,18,17,58),gett(8,21,18,34),"localhost","16777343","tianxiaochen01" ),

  ("pts/42",gett(9,8,17,43),gett(9,8,19,1),"localhost","16777343","tianxiaochen01" ),

  ("pts/43",gett(9,9,9,17,58),gett(9,9,18,34),"loginjp201z.prod.jp.local","1441857249","tianxiaochen01" ),

  ("pts/32",gett(9,14,9,10),gett(9,14,18,34),"localhost","16777343","tianxiaochen01" ),

  ("pts/49",gett(10,26,9,50),gett(10,26,18,3),"localhost","16777343","tianxiaochen01" ),
  ("pts/49",gett(10,27,10,30),gett(10,27,16,30),"localhost","16777343","tianxiaochen01" ),
  ("pts/49",gett(10,28,9,59),gett(10,28,17,7),"localhost","16777343","tianxiaochen01" ),
  ("pts/49",gett(10,29,9,54),gett(10,29,17,52),"localhost","16777343","tianxiaochen01" ),
  ("pts/49",gett(10,29,18,1),gett(10,29,18,15),"localhost","16777343","tianxiaochen01" ),
  ("pts/49",gett(10,30,9,48),gett(10,30,16,25),"localhost","16777343","tianxiaochen01" ),
  ("pts/49",gett(10,30,17,7),gett(10,30,18,30),"localhost","16777343","tianxiaochen01" )

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





