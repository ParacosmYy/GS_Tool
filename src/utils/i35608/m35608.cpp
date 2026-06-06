#include "i35608/m35608.h"
QVector<double> m35608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
