#include "m8452/m8452.h"
QVector<double> m8452::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
