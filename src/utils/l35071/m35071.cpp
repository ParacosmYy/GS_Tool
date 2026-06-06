#include "l35071/m35071.h"
QVector<double> m35071::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
