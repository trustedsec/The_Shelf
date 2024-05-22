

def tab_choice(val, line, **kwargs):
    results = []
    for key in kwargs['choices']:
        if key.startswith(val):
            results.append(key)
    return results