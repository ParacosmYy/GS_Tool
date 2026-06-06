#include "m37152/m37152.h"
QVector<double> m37152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
