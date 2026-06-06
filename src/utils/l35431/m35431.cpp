#include "l35431/m35431.h"
QVector<double> m35431::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
