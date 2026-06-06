#include "g12906/m12906.h"
QVector<double> m12906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
