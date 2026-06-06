#include "i8908/m8908.h"
QVector<double> m8908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
