#include "a32880/m32880.h"
QVector<double> m32880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
