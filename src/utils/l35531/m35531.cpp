#include "l35531/m35531.h"
QVector<double> m35531::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
