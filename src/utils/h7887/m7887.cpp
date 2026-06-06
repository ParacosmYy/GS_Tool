#include "h7887/m7887.h"
QVector<double> m7887::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
