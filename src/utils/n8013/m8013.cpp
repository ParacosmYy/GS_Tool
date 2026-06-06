#include "n8013/m8013.h"
QVector<double> m8013::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
