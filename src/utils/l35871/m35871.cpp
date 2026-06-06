#include "l35871/m35871.h"
QVector<double> m35871::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
