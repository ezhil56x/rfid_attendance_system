var SS = SpreadsheetApp.openById('GOOGLE_SHEET_ID');
var timezone = 'Asia/Kolkata';
var hours = 0;
var str = '';

function doPost(e) {
	var parsedData;

	try {
		parsedData = JSON.parse(e.postData.contents);
	} catch (f) {
		return ContentService.createTextOutput(
			'Error in parsing request body: ' + f.message
		);
	}

	if (parsedData !== undefined) {
		var sheet = SS.getSheetByName(parsedData.sheet_name); // sheet name to publish data to is specified in Arduino code
		var dataArr = parsedData.values.split(','); // creates an array of the values to publish

		var date = new Date(new Date().setHours(new Date().getHours() + hours));
		var Curr_Date = date
			.toLocaleDateString('en-GB', {
				day: '2-digit',
				month: '2-digit',
				year: 'numeric',
			})
			.replace(/\//g, '-');
		var Curr_Time = Utilities.formatDate(date, timezone, 'hh:mm:ss a');

		var value0 = dataArr[0]; // UID
		var value1 = dataArr[1]; // Regno
		var value2 = dataArr[2]; // First Name

		// read and execute command from the "payload_base" string specified in Arduino code
		switch (parsedData.command) {
			case 'insert_row':
				sheet.insertRows(2); // insert full row directly below header text
				var range = sheet.getRange('C2:F2'); // Set the range from column C to F to match the data array
				var values = [[value0, value1, value2, 'PRESENT']]; // Set the default value for the fourth column as "PRESENT"
				range.setValues(values); // Set the values for the range
				sheet.getRange('A2').setValue(Curr_Date); // publish current date to cell A2
				sheet.getRange('B2').setValue(Curr_Time); // publish current time to cell B2
				str = 'Success'; // string to return back to Arduino serial console
				break;
		}

		SpreadsheetApp.flush();
		return ContentService.createTextOutput(str);
	} else {
		return ContentService.createTextOutput(
			'Error! Request body empty or in incorrect format.'
		);
	}
}
