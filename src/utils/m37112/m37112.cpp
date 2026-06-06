#include "m37112/m37112.h"
QVector<double> m37112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
