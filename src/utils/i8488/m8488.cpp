#include "i8488/m8488.h"
QVector<double> m8488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
