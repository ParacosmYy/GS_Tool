#include "g8906/m8906.h"
QVector<double> m8906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
