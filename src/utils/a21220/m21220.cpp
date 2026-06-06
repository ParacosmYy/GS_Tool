#include "a21220/m21220.h"
QVector<double> m21220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
