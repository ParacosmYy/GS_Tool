#include "l28011/m28011.h"
QVector<double> m28011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
