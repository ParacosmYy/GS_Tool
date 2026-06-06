#include "l28591/m28591.h"
QVector<double> m28591::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
