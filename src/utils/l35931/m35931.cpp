#include "l35931/m35931.h"
QVector<double> m35931::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
