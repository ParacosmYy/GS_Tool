#include "i8028/m8028.h"
QVector<double> m8028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
