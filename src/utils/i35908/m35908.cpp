#include "i35908/m35908.h"
QVector<double> m35908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
