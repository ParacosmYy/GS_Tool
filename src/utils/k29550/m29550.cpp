#include "k29550/m29550.h"
QVector<double> m29550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
