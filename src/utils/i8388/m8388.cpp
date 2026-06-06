#include "i8388/m8388.h"
QVector<double> m8388::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
