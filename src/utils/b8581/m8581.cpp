#include "b8581/m8581.h"
QVector<double> m8581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
