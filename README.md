# Photo Legends App
This app is a label printing utility to create picture legends for photo exhibitions. It offers a table to enter the details of the photos. Entering the details might be simplified by dropping the photo onto the table where the app tries to retrieve the relevant information from the metadata available in the photo file. An heuristic approach is used to retrieve this information, so it might fail. Using the *Preview Button a preview of the labels can be seen and printed. The printed paper size and its orientation might be entered using the "yellow" button in the toolbar. The number of labels fitting the page horizontaly and vertically are calculated automatically. A specific feature of the app is, that it could add a QR code linking to a publishig site of the photo, where more information about the photo could be retrieved. 

As QR generator I am using https://www.nayuki.io/page/qr-code-generator-library\">https://www.nayuki.io/page/qr-code-generator-library. EasyExif from Mayank Lahiri served as start for the exif extractor and Oleh Aleinyk ideaes were used for the iptc-reader. Although thanks to Trapezoid_Dreams for the multiline edit idea in Qt.

##### Mac specific items:
To delete the prefrences you need to execute these two commands  
% defaults delete de.legoesprit.BilderLegenden  
% killall cfprefsd  
in the terminal.  


