#include "l35251/m35251.h"
QVector<double> m35251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
