#include "b8161/m8161.h"
QVector<double> m8161::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
