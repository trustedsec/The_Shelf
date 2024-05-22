Function CheckDomainNameValue()
{{checkmethod}}
End Function

Function key_domain
	If CheckDomainNameValue Like LCase("*{{domain}}*") Then
		key_domain = False
	Else
		key_domain = True
	End If
End Function
