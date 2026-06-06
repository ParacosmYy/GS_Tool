#include "p16735/m16735.h"
QVector<double> m16735::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
