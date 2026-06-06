#include "g21286/m21286.h"
QVector<double> m21286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
