    <?php
        $cookie_a1 = "1234567";
        $cookie_a2 = "cookie";
        $cookie_a3 = "这是cookie";
        $cookie_a4 = "测试测试";

        $form_a1 = "测试表单";
        $form_a2 = "testform";
        $form_a3 = "12345678";
        $form_a4 = "测试form";
        $form_a5 = "123123form";

        $file1 = "./_form_file.exe";
        $file2 = "./_file.exe";
        $form_filename1 = 'form_upload1';
        $form_filename2 = 'form_upload2';
        $filename = 'upload';

        if ($_REQUEST['act'] == 'cookie')
        {
            if ($_COOKIE['a1'] == $cookie_a1 && $_COOKIE['a2'] == $cookie_a2
                && $_COOKIE['a3'] == $cookie_a3 && $_COOKIE['a4'] == $cookie_a4)
            {
                $status = 200;
            }
            else
            {
                $status = 500;
            }
            header('Content-type: text/html; charset=utf-8',true,$status);
        }
        else if ($_REQUEST['act'] == 'form')
        {
            if ($_POST['a1'] == $form_a1 && $_POST['a2'] == $form_a2 && 
                $_POST['a3'] == $form_a3 && $_POST['a4'] == $form_a4 && 
                $_POST['a5'] == $form_a5
                && $_FILES[$form_filename1]['size'] && $_FILES[$form_filename1]['size'] == filesize($file1)
                && $_FILES[$form_filename2]['size'] && $_FILES[$form_filename2]['size'] == filesize($file1)
                )
            {
                move_uploaded_file($_FILES[$form_filename1]['tmp_name'], './'.$_FILES[$form_filename1]['name']);
                move_uploaded_file($_FILES[$form_filename2]['tmp_name'], './'.$_FILES[$form_filename2]['name']);
                $status = 200;
            }
            else
                $status = 500;

            header('Content-type: text/html; charset=utf-8',true,$status);
        }
        else if ($_REQUEST['act'] == 'get')
        {
            header('Content-type: text/html; charset=utf-8');
            for ($i=1;$i<11;$i++)
                echo $i.' - test测试123<br/>';
        }
        else if ($_REQUEST['act'] == 'file')
        {
            if ($_FILES[$filename]['size'] && $_FILES[$filename]['size'] == filesize($file2))
            {
                move_uploaded_file($_FILES[$filename]['tmp_name'], './'.$_FILES[$filename]['name']);
                $status = 200;
            }
            else
                $status = 500;
            header('Content-type: text/html; charset=utf-8',true,$status);
            echo 'uploadfile:'.$_FILES[$filename]['size'].'bytes,'.' originfile:'.filesize($file2).'bytes';
        } else {
            header('Content-type: text/html; charset=utf-8');
            setcookie("a1", $cookie_a1);
            setcookie("a2", $cookie_a2);
            setcookie("a3", $cookie_a3);
            setcookie("a4", $cookie_a4);
            $str_cookie = "<br/>（cookie串：a1=1234567; a2=cookie; a3=%E8%BF%99%E6%98%AFcookie; a4=%E6%B5%8B%E8%AF%95%E6%B5%8B%E8%AF%95";
           echo "
            <div style='width:600px;margin:50px auto;'>
            <center><h2>Sinet_lib测试</h2></center>
            <p>cookie：http://{$_SERVER['HTTP_HOST']}{$_SERVER['PHP_SELF']}?act=cookie{$str_cookie}</p>
            <p>表单：http://{$_SERVER['HTTP_HOST']}{$_SERVER['PHP_SELF']}?act=form，参数：a1,a2,a3,文件上传参数任意</p>
            <p>GET请求：http://{$_SERVER['HTTP_HOST']}{$_SERVER['PHP_SELF']}?act=get</p>
            <p>文件上传：http://{$_SERVER['HTTP_HOST']}{$_SERVER['PHP_SELF']}?act=file</p>
            </div>
<!--             <form action='./test_sinet.php?act=file' method='POST' enctype='multipart/form-data'>
                <input type='file' name='upload'/>
                <input type='submit'/>
            </form> -->
            ";
        }