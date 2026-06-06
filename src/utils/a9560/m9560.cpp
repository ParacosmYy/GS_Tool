#include "a9560/m9560.h"
QVector<double> m9560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
