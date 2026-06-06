#include "a35680/m35680.h"
QVector<double> m35680::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
