#include "l9251/m9251.h"
QVector<double> m9251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
