import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

public class EncodingConverter {
    
    public static void main(String[] args) {
        String inputString = "This is a sample string to be encoded.";
        Charset inputCharset = StandardCharsets.UTF_8;
        Charset outputCharset = StandardCharsets.ISO_8859_1;
        String outputString = convertEncoding(inputString, inputCharset, outputCharset);
        System.out.println("Input string: " + inputString);
        System.out.println("Output string: " + outputString);
    }
    
    public static String convertEncoding(String inputString, Charset inputCharset, Charset outputCharset) {
        byte[] inputBytes = inputString.getBytes(inputCharset);
        return new String(inputBytes, outputCharset);
    }
}
