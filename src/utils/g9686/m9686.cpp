#include "g9686/m9686.h"
QVector<double> m9686::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
