#include "e9104/m9104.h"
QVector<double> m9104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
