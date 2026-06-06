#include "p25735/m25735.h"
QVector<double> m25735::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
