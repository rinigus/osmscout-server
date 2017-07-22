##########################################################
## Support class to work with the hierarchy of properties

import os, codecs

class Hierarchy(object):
    base_dir = "hierarchy"

    @staticmethod
    def ignore(d):
        return os.path.exists(d + "/ignore")

    @staticmethod
    def get_from_parent(d, fname):
        dirs = d.split("/")
        base = Hierarchy.base_dir.split("/")
        i = len(dirs)-1
        while i > len(base):
            k = ""
            if d[0] == "/": k = "/"
            for j in range(i): k = os.path.join(k, dirs[j])
            k = os.path.join(k, fname)
            if os.path.exists(k):
                return k
            i -= 1
        return None

    @staticmethod
    def get_value(d, fname, parent=False):
        if os.path.exists(os.path.join(d, fname)):
            p = os.path.join(d, fname)
        else:
            p = Hierarchy.get_from_parent(d, fname)

        if p is not None:
            return codecs.open(p, 'r', "utf-8").read().strip()
        return None
    

    @staticmethod
    def get_base_name(d):
        base_list = Hierarchy.base_dir.split("/")
        rlist = d.split("/")
        ri = len(base_list)
        target = rlist[ri]
        ri+=1
        while ri < len(rlist):
            target += "-" + rlist[ri]
            ri += 1
        return target

    @staticmethod
    def get_full_name(d):
        dirs = d.split("/")
        base = Hierarchy.base_dir.split("/")
        i = len(dirs)-1
        name = None
        while i >= len(base):
            k = ""
            if d[0] == "/": k = "/"
            for j in range(i+1): k = os.path.join(k, dirs[j])
            k = os.path.join(k, "name")
            n = codecs.open(k, 'r', "utf-8").read().strip()

            if name is None: name = n
            else: name = n + "/" + name

            i -= 1

        return name

    @staticmethod
    def get_id(d):
        dirs = d.split("/")
        base = Hierarchy.base_dir.split("/")
        k = ""
        i = len(base)
        while i < len(dirs):
            k += dirs[i] + "/"
            i += 1
        return k[:-1]

    @staticmethod
    def has_postal(d):
        if os.path.exists(os.path.join(d, "postal_country")):
            return "MINE"
        if get_from_parent(d, "postal_country") is not None:
            return "PARENT"
        return None
    
    @staticmethod
    def get_postal(d):
        return Hierarchy.get_value(d, "postal_country",
                                   parent=True)
        
