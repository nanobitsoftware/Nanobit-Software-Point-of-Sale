#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <math.h>
//#include <crypt.h>
#include "Nano PoS.h"
#include "WJCrypt_Repo\lib\WjCryptLib_Sha512.h"





/* Go through a complex mathematical need to create a 512
 * byte salt so that we an use later for our passwords.
 * OVer kill? You're damnright it is. And I am okay with that.
 * Returns the 512 byte salt
 */
char* make_salt(void)
{
	char table[] = 
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"1234567890!@#$%^&*()`-=~_+"
		"\\|,./<>?\'\"\0";

	char q_table[250]; // Fifth random table

	char r_table[250];  // For random salt creation
	char s_table[250]; // Fourth random table

	char t_table[250];  // For picking randoms from random

	char u_table[250]; // Third Random table
	char v_table[250]; // Sixth. Should be 512+ now.

	static char ssalt[2048]; // We'll use 512 character salts.

	int i = 0;
	int r = 0;
	int c = (int)strlen(table);

	c--;

	int seed_count = 0; // So we know when to switch boxes.
	//char s1;  // Seed 1 item

	//char s2;  // Seed 2 item, xor.
	int mode = 0; // The mode we're using in seed creation
	ssalt[0] = '\0';

	for (i = 0; i < 2048; i++)
		ssalt[i] = '\0'; // Zero it out so we dont resuse it everytime.

	// Take the plain text first, check if its valid.
	//if (!plaintext)
	//	return NULL; // Not valid.
//	if (strlen(plaintext) < 4) // We need more than 4 chars. Yes, could be better.
	//	return NULL;

	// Let's salt the password.
	// Generate the salt:

	for (i = 0; i < c; i++)
	{
		// Let's do some random iterations, fill a new table with random data,
		// We'll go ahead and fill all 6 random tables here. in one go.

		if (!table[i])
			break;
		if (table[i] == '\0')
			break;

		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		q_table[i] = table[r];
		q_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		r_table[i] = table[r];
		r_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		s_table[i] = table[r];
		s_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		t_table[i] = table[r];
		t_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		u_table[i] = table[r];
		u_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		v_table[i] = table[r];
		v_table[i + 1] = '\0';
	}
	// Now, let's do 512 iterations, pulling from each table, making 512 bit
	// salt.
	mode = 0;
	seed_count = 0;
	c = (int)strlen(table) - 1;
	for (i = 0; i < 512; i++)
	{
		r = rand() % ((c + 1) - 0) + 0; // Random number.
		seed_count++;

		if (seed_count < c)
			mode = 0;
		else if (seed_count < (c * 2))// && seed_count < (c*3)))
			mode = 1;
		else if (seed_count < (c * 3))// && seed_count < (c*4)) )
			mode = 2;
		else if (seed_count < (c * 4))// && seed_count < (c*5))))
			mode = 3;
		else if (seed_count < (c * 5))// && seed_count < (c*6)))
			mode = 4;
		else if (seed_count < (c * 6)) //&& seed_count < (c*7)))
			mode = 5;
		else if (seed_count < (c * 7))
			mode = 6;
		else if (seed_count < (c * 8))
			mode = 7;

		switch (mode)
		{
		case 0:
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 2 ? q_table[r] : r_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
			break;
		}
		case 1:
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 5 ? s_table[r] : t_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
			break;
		}
		case 2:
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? u_table[r] : v_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
			break;
		}
		case 3:
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? q_table[r] : s_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
			break;
		}
		case 4:
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? r_table[r] : t_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
			break;
		}
		case 5:
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? q_table[r] : v_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
			break;
		}
		case 6:
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? u_table[r] : s_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
			break;
		}
		case 7:
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? u_table[r] : t_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
			break;
		}
		default:
			break;
		}
	}
	LOG("Salt: %s", ssalt);
	return ssalt;
}

char* hash_to_str(SHA512_HASH hash)
{
	static char str[SHA512_HASH_SIZE + 20];
	char t[100];
	int i;

	for (i = 0; i < SHA512_HASH_SIZE; i++)
		str[i] = '\0';

	for (i = 0; i < SHA512_HASH_SIZE; i++)
	{
		sprintf(t, "%02x", hash.bytes[i]);
		strcat(str, t);
		//	str[i] = hash.bytes[i];
	}
	str[SHA512_HASH_SIZE - 1] = '\0';
	return str;
}



extern Sha512Calculate(void  const* Buffer, uint32_t            BufferSize, SHA512_HASH* Digest);
// We WILL modify the salt during this operation.
// Do not send a copy of it. send the actual pointer
// to the real buffer.
char* crypt_password(char* plaintext, char* salt, CRYPTHASH* chash)
{
	int pwlen = 0;
	int saltlen = 0;
	int taglen = 0;
	int total_len = 0;
	char salt_tag[] = "::$SALT$::\0";
	char* pwsalt = NULL; // we'll malloc this.
	char r_salt[1024];
	SHA512_HASH hash;

	//SHA512_HASH  hash;          // [in]

	if (!plaintext || !salt)
	{
		LOG("No password or salt was given to crypt_password() -- Check this out.");
		return NULL;
	}

	// Let's take length of password and remove that length from the salt.
	// That way we have 512 bits at all times. If the password is larger
	// than 512...Well, We'll do something bad and just put the salt
	// at the end and make it 1024.

	pwlen = (int)strlen(plaintext);
	taglen = (int)strlen(salt_tag);
	if (pwlen <= 0)
		return NULL; // We need a password.
	if (pwlen > 512)
		saltlen = 512;
	else
		saltlen = (512 - (pwlen + taglen));

	if (saltlen < 0)
		saltlen = 0;
	// Truncate the salt.
	salt[saltlen] = '\0';

	total_len = pwlen + saltlen + taglen;
	// Create the new buffer for the pw + salt.
	pwsalt = malloc(sizeof(char*) * ((total_len)+10)); // Add ten just as a buffer.
	if (!pwsalt)
		GiveError("Unable to allocate memory for password encryption.", TRUE);

	sprintf(r_salt, "%s", salt);
	sprintf(pwsalt, "%s%s%s", plaintext, salt_tag, salt);
	pwsalt[total_len] = '\0';

	// We now have a password + salt that we can turn in to a hash.
	Sha512Calculate(pwsalt, strlen(pwsalt), &hash);
	
	LOG("\r\nPassword: %s, Hash((512)): %s\r\n",pwsalt, hash_to_str(hash));

	sprintf(chash->hash, "%s", hash_to_str(hash));

	sprintf(chash->password, "%s", plaintext);

	sprintf(chash->salt, "%s", r_salt);
	sprintf(chash->pw_salt, "%s", pwsalt);

	//return crypt(pwsalt);
	return NULL;
}