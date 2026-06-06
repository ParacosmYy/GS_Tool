#include "p18735/m18735.h"
QVector<double> m18735::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
