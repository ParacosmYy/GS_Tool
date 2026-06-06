#include "i17828/m17828.h"
QVector<double> m17828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
