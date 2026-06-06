#include "l35031/m35031.h"
QVector<double> m35031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
