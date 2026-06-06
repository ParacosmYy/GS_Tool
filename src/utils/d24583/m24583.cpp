#include "d24583/m24583.h"
QVector<double> m24583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
