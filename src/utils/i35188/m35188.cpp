#include "i35188/m35188.h"
QVector<double> m35188::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
