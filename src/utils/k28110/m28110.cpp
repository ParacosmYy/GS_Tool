#include "k28110/m28110.h"
QVector<double> m28110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
