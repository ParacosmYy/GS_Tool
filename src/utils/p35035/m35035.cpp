#include "p35035/m35035.h"
QVector<double> m35035::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
