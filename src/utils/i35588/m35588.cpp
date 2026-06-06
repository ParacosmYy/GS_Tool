#include "i35588/m35588.h"
QVector<double> m35588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
