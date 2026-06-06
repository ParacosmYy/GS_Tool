#include "l18251/m18251.h"
QVector<double> m18251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
