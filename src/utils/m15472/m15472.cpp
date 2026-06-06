#include "m15472/m15472.h"
QVector<double> m15472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
