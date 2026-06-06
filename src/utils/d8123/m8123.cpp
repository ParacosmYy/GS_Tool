#include "d8123/m8123.h"
QVector<double> m8123::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
