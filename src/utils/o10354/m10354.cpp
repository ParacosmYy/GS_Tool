#include "o10354/m10354.h"
QVector<double> m10354::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
